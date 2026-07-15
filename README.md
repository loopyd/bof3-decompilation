# bof3-harness

Reverse-engineering workspace for the independently loaded BOF3 executables
and reviewed EMI payloads.

## Quick path

Place user-owned US BIN/CUE media in `disks/`, then run:

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
bin/harness diff "$SOURCE"
```

Original bytes and canonical Splat assembly are authoritative. The reverse
mission is an orchestration aid; exact function matching and whole-payload
matching remain separate completion claims.

Keep a candidate only when it preserves factual, readable C89. Exact function
matching and whole-payload matching are separate completion claims.

## Common commands

| Command | Result |
| --- | --- |
| `just doctor` | Validate tracked configuration; safe before media extraction. |
| `just extract` | Build the native extractor and extract the disc. |
| `just unpack` | Unpack EMI archives from the extracted disc tree. |
| `bin/harness emi unpack` | Run Rust `emi-ex`; supports `--tool`. |
| `bin/harness discover` | Refresh `out/catalog/emi.json`. |
| `just build` | Run the historical PsyQ validation build serially. |
| `just check` | Run format checks, tests, Ruff, and doctor. |
| `just clean` | Remove `build/`; preserve evidence under `out/`. |

The global disc slot/LBA catalog covers EMI archives, executables, STR media,
and other disc files. EMI parsing is a separate layer that may depend on that
catalog; an EMI archive itself is never a decompilation target.

Durable layouts and source live in `config/splat/`, `config/symbols/`,
`src/exe/`, and `src/emi/`. See [setup](docs/setup.md),
[reverse engineering](docs/reverse-engineering.md),
[matching](docs/matching.md), and [troubleshooting](docs/troubleshooting.md).
