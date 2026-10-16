# BOF3 textures

`bof3-textures` catalogs captured US PlayStation graphics sources and exports
self-contained archive packages. It can edit raw VRAM upload words, their
4-bit or 8-bit index interpretations, and qualified palette-word banks within
their original allocations.

Full graphics support is incomplete. Category discovery lists the intended game
roles, but current resource roles, CLUT associations, sprite composition,
animation, compression, and runtime appearance remain unresolved. Strict coverage
and complete verification return exit 1. A successful raw upload edit does not
establish its appearance in the game.

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
checkout containing the two crates.

Extract the archive, then use the build command above from its `bof3-textures`
directory. The build requires the declared Rust toolchain and the locked Cargo
dependencies. Offline builds require those dependencies in the local Cargo cache.
The package does not vendor them or install tools.

## Inputs and commands

Inputs must match the captured `SLUS_004.22` qualification manifest in
[data/us-disc.json](data/us-disc.json). Supply an original EMI or the captured
extracted-disc root. Generated entry dumps are not additional original files.
Other releases and PSP data are unsupported. An individual archive whose bytes
occur at several original locations requires `--source` with its original path.
Tree qualification checks every directory except the generated entry directory
beside each original EMI. It ignores root XML, WAV, and `license_data.dat`
outputs, rejects other extra files and symlinks, and bounds inventory traversal
at 65,536 entries.

The captured `MAGIC00.STR`, `S_XA00.STR`, `VOICE.STR`, and `CAPCOM30.STR` files
are prefixes of their original XA disc extents. Their pinned hashes identify the
captured bytes, not complete original files. Catalog records mark them
`source-prefix-qualified`; coverage lists the missing extents and refuses to
qualify the tree as a complete original corpus. Full-source qualification remains
open with the disc extraction owner. The tool preserves these captured inputs
and does not repair or replace them.

```sh
build/tools/rust/bof3-textures/release/bof3-textures categories --json
build/tools/rust/bof3-textures/release/bof3-textures index out/extracted --output out/textures/index.json
build/tools/rust/bof3-textures/release/bof3-textures query out/textures/index.json --category unclassified-graphics --json
build/tools/rust/bof3-textures/release/bof3-textures unpack out/extracted/BIN/BATTLE/BATL_DRA.EMI --output out/textures/dragon-upload
build/tools/rust/bof3-textures/release/bof3-textures pack out/textures/dragon-upload --output out/textures/packed --json
build/tools/rust/bof3-textures/release/bof3-textures coverage out/extracted --strict --json
build/tools/rust/bof3-textures/release/bof3-textures verify out/extracted --json
```

Create the output parent first. Each output must be new and outside its input.
Exit 0 means the requested operation succeeded, exit 1 means required coverage or
verification remains incomplete, and exit 2 reports invalid input or an operation
failure. JSON errors are written to standard error. Query results and reports use
JSON on standard output, including when `--json` is omitted.

## Catalog

The index schema is `bof3.textures-index/v1`. A resource ID combines release,
original path, and EMI slot or file locator. Equal hashes retain distinct original
locations. Each record has its byte offset, size, hash, kind, game categories,
storage, decode and pack status, evidence, dependencies, known uses, and findings.
The catalog retains unknown entries and non-EMI files for further disposition.

Query supports `--category`, `--source`, `--asset`, `--storage`, `--kind`,
`--palette`, `--animation`, `--dependency`, and `--evidence`. Source matching uses
substrings. Other filters match exact identifiers. Filters combine with AND.
Resources remain in original-file and slot order. Empty results do not prove
that a category is absent from the game.

Query checks source content and inventory plus the compiled schema, qualification,
format, package, and mapping policy. It refuses a stale or altered catalog and
prints a rebuild instruction. Timestamps do not establish freshness. Rebuild the
executable after changing its interpretation sources.

## Edit packages

The manifest schema is `bof3.textures-package/v1`. The package includes complete
snapshots of the captured inputs, raw resources, its bound catalog, and coverage. A
qualified raw upload also has `words.json` and indexed `indices-4.png` and
`indices-8.png` views. These PNGs use diagnostic grayscale. They do not identify
a game palette, texture mode, UV layout, or assembled sprite.

A qualified palette bank has an editable `words.json` containing its exact
little-endian PSX color words. Preserve the word count. These banks have proven
source and staging addresses, but their draw CLUT selections and texture uses
remain unresolved. Editing a bank does not assign it to a diagnostic image.

Edit raw upload words as unsigned 16-bit values, or preserve a PNG's dimensions
and palette order while editing its indices. A 4-bit view accepts indices 0 to
15; an 8-bit view accepts indices 0 to 255. Keep unused holes zero. Changed views
that request different values for the same byte cause an edit-conflict error.
Unchanged views do not override edits. Changes to preservation-only resources,
allocation, source snapshots, catalog descriptors, or diagnostic palettes fail.

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
decoding, and palette-word inverses. The pack operation checks reconstruction
and unrelated bytes for its own outputs. Neither report establishes full category
or runtime acceptance.
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
unused tail values. Source-bank records and consumer bindings remain unresolved,
so this primitive does not yet add editable sprite packages to the CLI.

The `lzss` module implements the mode-2 byte stream consumed at
`SLUS_004.22@0x8014E8FC`. It retains the four-byte opaque prefix and uses
LSB-first flags with a 512-byte dictionary. The initial cursor is `0x1ee`;
only dictionary offsets below it are initialized to zero. Reading an unwritten
offset fails. Final-reference bytes beyond the declared image length are
reported separately and preserved during encoding. Unchanged streams remain
exact, and edited streams must fit the original allocation. Source-bank and
sprite bindings remain unresolved, so the CLI does not yet edit these streams.

Input files are bounded at 128 MiB, indexed images at 1,048,576 pixels, and each
codec process at 20 seconds. Invalid extents, malformed images, changed snapshots,
escaped package paths, missing members, and existing destinations cause failure.
Unresolved logical resources, categories, dependencies, inverse formats, and
runtime families keep full-support acceptance open.
