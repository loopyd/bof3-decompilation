# Getting started

## Prerequisites

- Rust and Cargo. The crate targets **edition 2024** with `rust-version = "1.85"`,
  and is validated with `cargo 1.98.1` (managed by mise: `mise up rust`).
- No other tooling. Dependencies are `clap`, `owo-colors`, `log`, `env_logger`,
  `serde` and `serde_json`, pinned in `Cargo.toml`.
- An extracted US BOF3 corpus under `out/extracted/BIN/` for corpus search,
  or one US EMI archive for the individual archive commands.

## Build and publish

The harness looks for the binary at
`build/tools/rust/bof3-text/release/bof3-text`, matching `repo_layout().bof3_text_bin`.
Publish it with the same per-crate target directory the other Rust tools use:

```sh
CARGO_TARGET_DIR=build/tools/rust/bof3-text \
  cargo build --release --manifest-path tools/rust/bof3-text/Cargo.toml
```

A plain `cargo build --release` inside the crate directory only refreshes
`tools/rust/bof3-text/target/`, which the harness does not read.

## Run

Prepare search artifacts once after extracting the disc, then search by phrase:

```sh
bin/harness text prepare
bin/harness text search --grep "it'll be a good" -i --limit 20
bin/harness text search --grep McNeil -i --class dialogue --json
```

`prepare` derives vocabulary from verified dialogue/battle rows, initializes the
payload inventory from the crate's recorded policy when absent, rebuilds ownership
windows using the repository's reviewed target records, and builds a filtered
index. It preserves an existing payload inventory and validates it. No generated
text artifacts or filter opt-outs are needed to bootstrap. Original archives are
read only. Use `--root DIR` for a different extracted corpus; later searches must
use the same root. See [commands.md](commands.md) for alternate artifact paths.

Individual archive commands also work without a prepared vocabulary:

```sh
bin/harness text --help          # modes and flags
bin/harness text index ARCHIVE.EMI
bin/harness text extract ARCHIVE.EMI -o OUT.json
bin/harness text validate OUT.json
bin/harness text pack --original ARCHIVE.EMI --text OUT.json -o OUT.EMI
```

`bin/harness text` is a thin Python dispatcher (`harness.commands.text`) that execs the
binary, mirroring `bin/harness emi archive`. If the binary is missing it **builds it once with
cargo** before running, so a fresh checkout works without a prior `just setup`; you can also run
the binary directly. Documents are always JSON — there is no second codec.

## Environment overrides

| Variable | Effect |
| --- | --- |
| `PSX_BOF3_TEXT` | Absolute path to a different `bof3-text` binary; the harness uses it instead of the published one |
| `RUST_LOG` | Overrides the level implied by `-v` flags (`env_logger` filter syntax) |
| `NO_COLOR` | Disables colour, like `--no-color` |

## Verify an installation

```sh
# the tool's own corpus gate: every text subfile must reproduce exactly
bin/harness text verify

# or round trip one archive by hand and compare the bytes
bin/harness text extract out/extracted/BIN/WORLD00/AREA000.EMI -o /tmp/a.json
bin/harness text pack --original out/extracted/BIN/WORLD00/AREA000.EMI \
    --text /tmp/a.json -o /tmp/a.EMI
cmp out/extracted/BIN/WORLD00/AREA000.EMI /tmp/a.EMI && echo "byte-identical"
```

## Next

- [commands.md](commands.md) for every mode and flag.
- [formats.md](formats.md) for what the documents mean.
- [logging.md](logging.md) for verbosity and JSON logs.
