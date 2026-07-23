# bof3-disk v2

Canonical repository implementation of the disc extraction and checksum path.
The imported C++ implementation remains available as a parity oracle and for
the rebuild behavior not yet implemented in Rust.

The current pure-Rust vertical slice extracts ordinary ISO9660 files
byte-for-byte from cooked 2048-byte images and raw 2352-byte Mode 1/Mode 2
images. CUE files with a local first `FILE` entry are accepted. It validates
sector ranges, both-endian ISO9660 fields, directory records, and output names
before writing.

```sh
CARGO_TARGET_DIR=build/tools/rust/bof3-disk \
  cargo build --manifest-path tools/rust/bof3-disk/Cargo.toml --release
CARGO_TARGET_DIR=build/tools/rust/bof3-disk \
  cargo test --manifest-path tools/rust/bof3-disk/Cargo.toml
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

`disk-equivalence` hashes and compares every ordinary ISO9660 payload emitted
by v2. It does not claim whole-disc artifact parity.

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
| Rebuild/CUE | Pending |

Extraction/checksum commands are canonical. Rebuild remains explicitly
unsupported until the remaining rows are parity-tested; the CLI fails rather
than emitting an unverified disc image.

## Dependencies

Generic formats and plumbing use mature crates: `serde`/`serde_json` for typed
metadata, `quick-xml` for escaped XML serialization, `md-5`/`sha2` for
streaming portable checksums, `hound` for RIFF/WAVE serialization, and
`tempfile` for isolated tests. Custom Rust is limited to PlayStation-specific
raw-sector, XA/CDDA range, CUE-layout, and image semantics.
