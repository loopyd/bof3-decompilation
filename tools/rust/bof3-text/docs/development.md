# Development

## Module layout

| Module | Owns |
| --- | --- |
| `main.rs` | Entry point: passes argv to `cli::main` |
| `cli.rs` | clap argument model, dispatch, output writing, document loading |
| `logging.rs` | Logger setup, verbosity mapping, colour policy, style helpers |
| `command.rs` | Serializer/deserializer for the editable grammar |
| `syntax.rs` | Document model, the JSON codec and the linter |
| `coverage.rs` | Complete extracted-tree accounting and unresolved ranges |
| `corpus.rs` | Verified bank, record name and executable corpus export with per-source results |
| `analysis.rs` | Original byte extents, commands, operands and shared rows |
| `executable.rs` | Pinned US exception ASCII allocations, references and fixed-capacity writes |
| `records.rs` | Pinned US equipment and ability name fields and literal-only fixed-width writes |
| `characters.rs` | Separate START and STATUS default character name allocations and writes |
| `digest.rs` | Shared SHA-256 content identities |
| `extract.rs` | EMI TOC discovery, bank parsing, document construction, fingerprint |
| `pack.rs` | Edit planning, span-preserving rebuild, payload replacement |
| `query.rs` | Index records, row queries, command accounting |
| `models.rs` | Dialogue/bank/terminator models, the generic `Command` model and per-command `CommandClass` implementations, the character table |

Dependency direction: `cli` → {`extract`, `pack`, `query`, `syntax`, `logging`};
`pack` → `syntax`; `syntax` → `command`; everything → `models`. `models` depends
on nothing local, so the shared vocabulary stays importable and cycle-free.

## Toolchain

The crate uses edition 2024 and requires Rust 1.85 or newer. Build with the
versions selected by `Cargo.toml` and `Cargo.lock`:

```sh
CARGO_TARGET_DIR=build/tools/rust/bof3-text \
  cargo build --locked --release --manifest-path tools/rust/bof3-text/Cargo.toml
```

Building with a newer compiler must not require source or dependency changes.

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
7. **Source binding.** Version 3 documents require the entire archive SHA-256,
   original row fingerprint and an explicit source entry. Packing verifies the
   archive path, digest, entry, load address and class before writing.
   Executable ASCII documents use their own schema and bind the full source
   digest, canonical path, pinned layout and unchanged pointer references.

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
bin/harness text verify               # require every selected entry to round-trip
bin/harness text search --verify-entries   # require every indexed entry to reproduce

# the whole gate, including the docs drift check
just check
```

`just check` includes the documentation drift check. Corpus commands report
their own coverage and failures; successful unit checks do not replace them.
The [dialogue specification](../../../../docs/specs/formats/dialogue-text.md)
records the verified format and remaining limits.

## Conventions

- New dependencies require specific user authorization.
- Errors are `models::Error` variants. `Invalid` is a usage/data error; `Unsupported`
  means the data is outside the verified model. Both print as `error: …` and exit 1.
- Public items carry a doc comment; command output goes to stdout and logs to stderr.
