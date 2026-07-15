# emi-ex v2

Canonical repository implementation of the Capcom EMI extractor. The imported
C++ implementation remains available only as a parity oracle.

```sh
cargo build --manifest-path third_party/emi-ex-v2/Cargo.toml --release
cargo test --manifest-path third_party/emi-ex-v2/Cargo.toml
build/third_party/emi-ex-v2/release/emi-ex extract [-e] [-o DIR] ARCHIVE.EMI [INDEX]
build/third_party/emi-ex-v2/release/emi-ex pack -o ARCHIVE.EMI -J DIR/emi.json DIR
```

`just clean` in this directory runs Cargo cleanup for this crate only; it never
removes repository `out/` or another crate's artifacts.

From the repository root, compare v2 with the active v1 extractor on one real
archive or the complete extracted disc tree:

```sh
just --justfile third_party/emi-ex-v2/Justfile equivalence out/extracted/BIN/ETC/GAME.EMI
just --justfile third_party/emi-ex-v2/Justfile equivalence-all out/extracted
```

The extractor validates the header, TOC bounds, payload bounds, and entry
index before writing. Entries begin at `0x800` and each payload advances to the
next `0x800`-byte sector, matching the shipped archives and v1 extractor.

## Parity matrix

| Contract | Status |
| --- | --- |
| Parse/extract all or one entry | Fixture and real-archive extraction parity tested |
| Typed extensions and JSON manifest | Implemented; manifest formatting fixture-tested |
| Explicit-input packing | Implemented; byte-exact fixture test |
| Manifest round-trip packing | Implemented; byte-exact fixture test |
| Folder/glob packing and type guessing | Implemented and fixture-tested |
| v1 dry-run/delete-original behavior | Implemented and fixture-tested |
| Full v1 CLI/API error and default behavior | Pending |

The crate uses `serde`/`serde_json` for the standard manifest contract and
keeps only the small EMI binary layout in custom Rust. Extraction is canonical
after byte-identical comparison with the legacy tool across all 880 BOF3 EMI
archives. Pack/CLI edge behavior remains covered primarily by fixtures.
