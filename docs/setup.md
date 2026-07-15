# Setup

> Set up the supported BOF3 reverse-engineering workspace.

## Contract

- Host: Linux x86_64.
- Game input: user-owned US BIN/CUE media under `inputs/disc/`; it is ignored by Git.
- Generated files: `build/`, `out/`, and `toolchains/`.
- Tracked binary layout: `config/splat/` and `config/symbols/`.

## Quick path

```bash
just setup
bin/harness doctor --strict
```

`just setup` initializes pinned submodules, prepares the PSX tools, stages PsyQ
4.7, extracts the disc, unpacks
EMI archives, and refreshes the catalog. SDK files remain ignored and must not
be committed.

Run `just psyq` separately only when restaging the SDK.

## Choose the starting state

- Fresh or newly cloned checkout: run `just doctor` first, place the US BIN/CUE
  media in `inputs/disc/`, then run `just setup`.
- Existing checkout with generated media: run `just doctor`, then `just check`
  or `just build`; use `just discover` only when the extracted catalog changed.
- Existing checkout with a missing generated stage: run `just extract` for disc
  extraction or `just unpack` for EMI unpacking, then `just discover`.

`just doctor` validates tracked configuration and does not require disc media or
an existing catalog. Strict mode also accepts missing quarantined payloads, but
active target payloads must be present and hash-valid.

Use these day-to-day targets after setup:

```bash
just extract
just unpack
just discover
just build
just check
just format
```

`just build` compiles historical PsyQ objects serially. Parallel compilation can
race a transient compiler output and leave a misleading partial archive.

To rerun only disc extraction or EMI unpacking:

```bash
just extract
just unpack
```

`just extract` builds the native extraction tools and extracts the disc into
`out/extracted/`. `just unpack` expects that extracted tree and writes unpacked
EMI entries below the same generated root. Use `just setup` for the complete
toolchain, extraction, unpack, and catalog workflow.

## Local input

Keep the original US disc files in `inputs/disc/`. Do not commit game data, extracted
payloads, SDK files, or generated analysis output. See
[../inputs/disc/README.md](../inputs/disc/README.md) for the input boundary.

## Verification

After extraction, `bin/harness discover` writes `out/catalog/emi.json`.

Optional analysis and last-mile matching tools are not required for the core
setup. Install or enable them only when the active function needs them.
