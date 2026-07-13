# Setup

> Set up the supported BOF3 reverse-engineering workspace.

## Contract

- Host: Linux x86_64.
- Game input: user-owned US BIN/CUE media under `disks/`; it is ignored by Git.
- Generated files: `build/`, `out/`, and `toolchains/`.
- Tracked binary layout: `config/splat/` and `config/symbols/`.

## Quick path

```bash
just setup
bin/harness doctor --strict
```

`just setup` prepares the required submodules and PSX tools, stages PsyQ 4.7,
extracts the disc, unpacks EMI archives, and refreshes the catalog. SDK files
remain ignored and must not be committed.

Run `just psyq` separately only when restaging the SDK.

Use these day-to-day targets after setup:

```bash
just extract
just build
just check
just format
```

`just build` compiles historical PsyQ objects serially. Parallel compilation can
race a transient compiler output and leave a misleading partial archive.

To rerun only extraction or rematerialize normalized images:

```bash
just extract
bin/harness emi unpack
bin/harness normalize
```

`bin/harness normalize` restores both executable load images and all tracked
EMI target images from the extracted payloads; it does not recover missing disc
input.

## Local input

Keep the original US disc files in `disks/`. Do not commit game data, extracted
payloads, SDK files, or generated analysis output. See
[../disks/README.md](../disks/README.md) for the input boundary.

## Verification

After extraction, `bin/harness scan` writes `out/catalog/emi.json` and
`bin/harness index build` writes `out/index/harness.sqlite`.

Optional analysis and last-mile matching tools are not required for the core
setup. Install or enable them only when the active function needs them.
