# bof3-disk v2

Canonical repository implementation of disc extraction, checksums, fixed-size
file replacement and synthetic rebuilds.

The pure-Rust extraction path reads ordinary ISO9660 files byte-for-byte from
cooked 2048-byte images and raw 2352-byte Mode 1/Mode 2 images. CUE files with
a local first `FILE` entry are accepted. It validates sector ranges,
both-endian ISO9660 fields, directory records, and output names before writing.

CD-XA extents use the ISO directory byte length rounded up to 2048-byte logical
blocks. Raw XA extraction writes 2336 bytes per sector, including the subheaders
and trailing sector data. Extracted files can therefore exceed their ISO
directory byte length.
`disc_lba.json` keeps the ISO byte length in `bytes` and the logical block count
in `length`.

```sh
CARGO_TARGET_DIR=build/tools/rust/bof3-disk \
  cargo build --manifest-path tools/rust/bof3-disk/Cargo.toml --release
CARGO_TARGET_DIR=build/tools/rust/bof3-disk \
  cargo test --locked --manifest-path tools/rust/bof3-disk/Cargo.toml
build/tools/rust/bof3-disk/release/bof3-disk extract \
  -i inputs/external/disc.cue -o out/extracted-v2
```

`just clean` in this directory runs Cargo cleanup for this crate only; it never
removes repository `out/` or another crate's artifacts.

The shared v2 task interface runs formatting, Clippy, tests, and fixture-backed
v1 comparison without changing the active harness integration:

```sh
just --justfile tools/rust/bof3-disk/Justfile check
just --justfile tools/rust/bof3-disk/Justfile equivalence inputs/external/disc.cue
```

`disk-equivalence` hashes and compares every v2 ISO9660 payload with the
corresponding v1 output. It does not claim whole-disc artifact parity.

## Synthetic rebuild contract

`rebuild -i DIR -o IMAGE.iso` emits a deterministic cooked 2048-byte-sector
ISO9660 image from a non-empty directory containing only top-level regular
files. File names must use uppercase ASCII letters, digits, `.`, or `_`; entries
are sorted bytewise by name. The image contains a PVD, terminator, root
records, and the files' exact payload bytes. It does not write a CUE sheet.

This is only a checked-in synthetic-fixture contract. It is **not** retail-disc
parity, raw BIN/CUE output, XA/CDDA support, nested-directory support,
mkpsxiso XML compatibility, or evidence about authorized local media.

## Replace a file in a copy

Use `replace` to copy an existing image and replace one ordinary ISO9660 file
without changing its size or disc location:

```sh
build/tools/rust/bof3-disk/release/bof3-disk replace \
  -i inputs/external/disc.cue -o out/edited-track.bin \
  --path BIN/ETC/START.EMI --replacement out/edited-start.EMI
```

The ISO path is relative, case-sensitive and omits the `;1` version suffix.
The replacement must match the directory's file size and fit within 64 MiB.
The destination must be new. Inputs remain unchanged. A write failure may leave
a partial destination; use a new path when retrying.

Cooked 2048-byte images and raw 2352-byte Mode 2 Form 1 sectors are supported.
Raw sectors must have matching subheaders and valid original EDC/ECC. The command
repairs EDC/ECC for changed sectors and preserves headers, final-sector padding,
unrelated sectors and extent locations. It refuses directories, XA files,
extended attributes, interleaving, associated files, multi-extent records and
overlapping extents. Raw Mode 1 and Mode 2 Form 2 replacement are unsupported.

For CUE input, the output contains a copy of the data-track file. Create a separate
disposable CUE sheet with the original track declarations, pointing its data file
to the new image and its other files to the unchanged originals. The command does
not generate a CUE sheet, relocate files or rebuild a retail disc.

## Parity matrix

| Contract | Status |
| --- | --- |
| Cooked ISO files | Fixture-backed byte parity |
| Raw Mode 1/2 Form 1 files | Fixture-backed byte parity |
| CUE parsing | Multi-file tracks, modes, INDEX 00/01, and pregaps fixture-tested |
| Checksums and verification | Portable in-process MD5/SHA-256; integration-tested |
| `disc_lba.json` rows | Typed serde schema; ISO byte and logical block counts; v1 lexical keys and LBA order |
| Project XML | Provisional; not yet mkpsxiso-compatible |
| License sectors | Raw 2336-byte-sector extraction implemented and fixture-tested |
| XA file extraction | CD-XA attributes and 2048-byte logical extents parsed; 2336-byte payload extraction per sector fixture-tested; real-disc parity pending |
| CDDA/WAV extraction | Pure-Rust stereo 44.1 kHz WAV conversion fixture-tested; real-disc parity pending |
| Synthetic cooked ISO rebuild | Fixture-backed deterministic contract |
| Fixed-size file replacement | Cooked images and raw Mode 2 Form 1 sectors; preserves disc layout |
| Retail-disc / raw BIN-CUE rebuild | Unsupported |

## Dependencies

Generic formats and plumbing use mature crates: `serde`/`serde_json` for typed
metadata, `quick-xml` for escaped XML serialization, `md-5`/`sha2` for
streaming portable checksums, `hound` for RIFF/WAVE serialization, and
`tempfile` for isolated tests. Custom Rust is limited to PlayStation-specific
raw-sector, XA/CDDA range, CUE-layout, image semantics, and the deliberately
small synthetic ISO writer.
