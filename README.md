# bof3-harness

Reverse-engineering workspace for the independently loaded BOF3 executables
and reviewed EMI payloads.

## Quick path

Place user-owned US BIN/CUE media in `inputs/disc/`, then run:

```sh
just setup
bin/harness doctor --strict
bin/harness targets
```

`just setup` builds the canonical Rust extractors, extracts the disc, unpacks
EMI archives, and refreshes the catalog. Generated state stays under
`build/`, `out/`, and `toolchains/`.

## Reverse one function

```sh
bin/harness targets "$TARGET"
bin/harness reverse "$TARGET"@"$ADDRESS" --run
bin/harness diff "$SOURCE" --llm
bin/asmdiff "$SOURCE"
bin/permute "$SOURCE" -j "$BOUNDED_JOBS"
```

Original bytes and canonical Splat assembly are authoritative. `--run` launches
one bounded OpenCode mission and records generated prompt/output evidence under
`out/reverse/`; exact function matching and whole-payload matching remain
separate completion claims.

Keep a candidate only when it preserves factual, readable C89. Exact function
matching and whole-payload matching are separate completion claims.

## Common commands

| Command | Result |
| --- | --- |
| `just doctor` | Validate tracked configuration; safe before media extraction. |
| `just extract` | Build the native extractor and extract the disc. |
| `just unpack` | Unpack EMI archives from the extracted disc tree. |
| `just pack` | Repack unpacked EMI manifests into the extracted disc tree. |
| `bin/harness emi unpack` | Run Rust `emi-ex`; supports `--tool`. |
| `bin/harness discover` | Refresh `out/catalog/emi.json`. |
| `just build` | Run the historical PsyQ validation build serially. |
| `just check` | Run format checks, tests, Ruff, and doctor. |
| `just rebuild TARGET` | Write a transitional rebuilt target image under `out/rebuilt/`. |
| `just verify [TARGET]` | Compare rebuilt target bytes, length, and SHA1. |
| `just clean` | Remove `build/`; preserve evidence under `out/`. |

The global disc slot/LBA catalog covers EMI archives, executables, STR media,
and other disc files. EMI parsing is a separate layer that may depend on that
catalog; an EMI archive itself is never a decompilation target.

Durable layouts and source live in `config/splat/`, `config/symbols/`,
`src/exe/`, and `src/emi/`. See [setup](docs/setup.md),
[reverse engineering](docs/reverse-engineering.md),
[matching](docs/matching.md), and [troubleshooting](docs/troubleshooting.md).
