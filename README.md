# bof3-harness

Reverse-engineering workspace for the independently loaded BOF3 executables
and reviewed EMI payloads.

## Quick path

Place user-owned US BIN/CUE media in `disks/`, then run:

```sh
just setup
bin/harness doctor --strict
bin/harness target list
```

`just setup` builds the canonical Rust extractors, extracts the disc, unpacks
EMI archives, normalizes both PS-X executables and every tracked EMI target,
and refreshes the catalog and evidence index. Generated state stays under
`build/`, `out/`, and `toolchains/`.

## Reverse one function

```sh
bin/harness target show "$TARGET"
bin/harness next "$TARGET"
bin/harness lift "$TARGET" "$ADDRESS"
# Edit the emitted src/.../func_XXXXXXXX.c file.
bin/harness diff "$SOURCE"
```

Original bytes and canonical Splat assembly are authoritative. Once the target,
load address, boundary, and compiler command are proven, use generated context
or bounded permutation when they help:

```sh
bin/harness context build "$TARGET" "$ADDRESS"
bin/harness permute "$SOURCE" --jobs 8
```

Keep a candidate only when it preserves factual, readable C89. Exact function
matching and whole-payload matching are separate completion claims.

## Common commands

| Command | Result |
| --- | --- |
| `just extract` | Extract the disc, then unpack every EMI archive. |
| `bin/harness emi unpack` | Run Rust `emi-ex`; supports `--tool`. |
| `bin/harness normalize` | Restore normalized EXE and tracked EMI images. |
| `bin/harness scan` | Refresh `out/catalog/emi.json`. |
| `bin/harness index build` | Refresh the repository evidence graph. |
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
