# Development

## Module layout

| Module | Owns |
| --- | --- |
| `main.rs` | Entry point: passes argv to `cli::main` |
| `cli.rs` | clap argument model, dispatch, output writing, document loading |
| `logging.rs` | Logger setup, verbosity mapping, colour policy, style helpers |
| `command.rs` | Serializer/deserializer for the editable grammar |
| `syntax.rs` | Document model, the JSON codec and the linter |
| `extract.rs` | EMI TOC discovery, bank parsing, document construction, fingerprint |
| `pack.rs` | Edit planning, span-preserving rebuild, payload replacement |
| `query.rs` | Index records, row queries, command accounting |
| `models.rs` | Dialogue/bank/terminator models, the generic `Command` model and per-command `CommandClass` implementations, the character table |

Dependency direction: `cli` → {`extract`, `pack`, `query`, `syntax`, `logging`};
`pack` → `syntax`; `syntax` → `command`; everything → `models`. `models` depends
on nothing local, so the shared vocabulary stays importable and cycle-free.

## Toolchain

The crate targets **edition 2024** and declares `rust-version = "1.85"` as its
minimum. The user account manages Rust with mise (`rust = "latest"`, currently
1.98.1) and the build is verified with that toolchain:

```sh
mise up rust                       # keep the global toolchain current
RUSTUP_TOOLCHAIN=1.98.1 CARGO_TARGET_DIR=build/tools/rust/bof3-text \
  cargo build --release --manifest-path tools/rust/bof3-text/Cargo.toml
```

Dependencies stay on their pinned versions in `Cargo.toml`; a build with a newer
compiler must not require source changes, and this one did not.

## Invariants

Changes to the format code must preserve these; each is exercised by the
verification commands below.

1. **Byte-exactness.** An unedited extract → pack round trip reproduces the
   archive byte-for-byte. This holds because the offset tables are
   re-emitted verbatim and only row spans are rewritten.
2. **Fixed allocation.** The payload size never changes, so no relocation logic is
   needed. An edit that does not fit its row's span is refused.
3. **No silent edits.** A row with no extent cannot receive text; slots sharing one
   extent must be edited identically; an edit that does not fit its extent — including
   a choice row's option list — is refused rather than truncated.
4. **No guessing.** Bytes outside the known character/command tables become
   `{byte(0xNN)}`. A command byte whose operands are cut off by the terminator is
   a raw byte, not a truncated command. `0x00` is refused as an operand.
5. **Fail closed.** Unsupported input produces an error naming the bank, row or
   offset; nothing is approximated or dropped.
6. **Create-new outputs.** The destination is opened with `create_new`, so the
   source archive cannot be reached through another spelling, symlink or hard link.
7. **Fingerprint.** A document's `block_fingerprint` must match the block being
   packed, which catches documents taken from a different archive or entry.

## Extending the format

- **A new command token.** Add a `command_class!` entry in `models.rs` and a row in
  `COMMAND_CLASSES`. `command.rs` and the linter pick it up automatically. Add it
  to the format documentation and to the US-consumer dispatch evidence before
  relying on its meaning; an unverified token must stay `{byte(0xNN)}`.
- **A new text class.** Add the variant and the load argument in `models.rs`
  (`TextClass`, `TextClass::from_load_argument`, `name`, `load_address`, `parse`);
  discovery and selection pick it up. That enum is the single owner of class
  identity.
- **A new bank shape.** Bank parsing lives in `extract::parse_section` and
  `bank_bounds`; a two-bank subfile keeps its 8-byte header in `Section::prefix` and
  packing re-emits it. Every byte after an offset table must belong to a row extent, and
  a header that does not validate must be refused rather than absorbed — that is how the
  seven coincidental `0x800E3800` markers are rejected.

## Validation

```sh
# formatting and build, warning-free
cargo fmt --manifest-path tools/rust/bof3-text/Cargo.toml -- --check
cargo build --release --manifest-path tools/rust/bof3-text/Cargo.toml

# the crate's unit tests (nothing else runs them)
cargo test --locked --release --manifest-path tools/rust/bof3-text/Cargo.toml \
    --target-dir build/tools/rust/bof3-text

# corpus gates: every text subfile round-trips, and the index describes the archives
bin/harness text prepare              # bootstrap missing artifacts after disc extraction
bin/harness text build-index          # rebuilds only when stale; a no-op when fresh
bin/harness text verify               # expect 244/244 (dialogue 200/200, battle 44/44)
bin/harness text search --verify-entries   # expect 46057/46057 reproduced

# the whole gate, including the docs drift check
just check
```

`just check` runs these plus the documentation drift check, so a change that leaves the docs,
the index or the round trip inconsistent fails the gate rather than being discovered later. The
full evidence set, including the gap audit and the class-promotion refutation, is indexed in
`out/text-format/README.md` at the repository root.

## Conventions

- No new dependency without a reason; versions are pinned in `Cargo.toml`.
- Errors are `models::Error` variants. `Invalid` is a usage/data error; `Unsupported`
  means the data is outside the verified model. Both print as `error: …` and exit 1.
- Public items carry a doc comment; command output goes to stdout and logs to stderr.
