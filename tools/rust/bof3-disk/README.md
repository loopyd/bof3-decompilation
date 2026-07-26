# bof3-disk v2

Canonical repository implementation of the disc extraction, checksum, and
synthetic rebuild path. The imported C++ implementation remains available as a
parity oracle for contracts not implemented here.

The pure-Rust extraction path reads ordinary ISO9660 files byte-for-byte from
cooked 2048-byte images and raw 2352-byte Mode 1/Mode 2 images. CUE files with
a local first `FILE` entry are accepted. It validates sector ranges,
both-endian ISO9660 fields, directory records, and output names before writing.

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

## Parity matrix

| Contract | Status |
| --- | --- |
| Cooked ISO files | Fixture-backed byte parity |
| Raw Mode 1/2 Form 1 files | Fixture-backed byte parity |
| CUE parsing | Multi-file tracks, modes, INDEX 00/01, and pregaps fixture-tested |
| Checksums and verification | Portable in-process MD5/SHA-256; integration-tested |
| `disc_lba.json` rows | Typed serde schema, v1 lexical keys/LBA order/XA length golden-tested |
| Project XML | Provisional; not yet mkpsxiso-compatible |
| License sectors | Raw 2336-byte-sector extraction implemented and fixture-tested |
| XA file extraction | CD-XA directory attributes parsed; 2336-byte extent counting and payload extraction fixture-tested; real-disc parity pending |
| CDDA/WAV extraction | Pure-Rust stereo 44.1 kHz WAV conversion fixture-tested; real-disc parity pending |
| Synthetic cooked ISO rebuild | Fixture-backed deterministic contract |
| Retail-disc / raw BIN-CUE rebuild | Unsupported |

## Dependencies

Generic formats and plumbing use mature crates: `serde`/`serde_json` for typed
metadata, `quick-xml` for escaped XML serialization, `md-5`/`sha2` for
streaming portable checksums, `hound` for RIFF/WAVE serialization, and
`tempfile` for isolated tests. Custom Rust is limited to PlayStation-specific
raw-sector, XA/CDDA range, CUE-layout, image semantics, and the deliberately
small synthetic ISO writer.
