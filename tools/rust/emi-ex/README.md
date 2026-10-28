# emi-ex v2

Canonical repository implementation of the Capcom EMI extractor. The imported
C++ implementation remains available only as a parity oracle.

```sh
cargo build --manifest-path tools/rust/emi-ex/Cargo.toml --release
cargo test --manifest-path tools/rust/emi-ex/Cargo.toml
build/tools/rust/emi-ex/release/emi-ex extract [-e] [-o DIR] ARCHIVE.EMI [INDEX]
build/tools/rust/emi-ex/release/emi-ex pack -o ARCHIVE.EMI -J DIR/emi.json DIR
```

`just clean` in this directory runs Cargo cleanup for this crate only; it never
removes repository `out/` or another crate's artifacts.

From the repository root, compare v2 with the active v1 extractor on one real
archive or the complete extracted disc tree:

```sh
just --justfile tools/rust/emi-ex/Justfile equivalence out/extracted/BIN/ETC/GAME.EMI
just --justfile tools/rust/emi-ex/Justfile equivalence-all out/extracted
```

The extractor validates the header, TOC bounds, payload bounds, and entry
index before writing. Entries begin at `0x800` and each payload advances to the
next `0x800`-byte sector, matching the shipped archives and v1 extractor.

## Preserve an original archive

Replace one entry using its zero-based index and a separate, new output path:

```sh
build/tools/rust/emi-ex/release/emi-ex replace-entry ORIGINAL.EMI INDEX PAYLOAD.bin -o OUTPUT.EMI
```

The replacement must have the original entry's size and first four payload bytes.
For entries shorter than four bytes, the entire payload must stay unchanged.
The command preserves the complete table, entry placement, padding, trailer and
every unrelated byte. It rejects invalid archives, unknown indices, size or prefix
changes, repeated output options and existing output paths. Inputs stay unchanged.
Generic `pack` constructs an archive and can normalize padding; use `replace-entry`
when the original archive's opaque bytes must survive.

The library exposes this operation as `ArchiveImage::replace_entry`. Its separate
`replace_entries` API allows size changes within sector allocation and updates
table fields, so it has a different preservation contract.

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
