# Setup

Use `bin/*` as the primary tool interface. `make` is intentionally small and
only covers setup, test, format, build, and high-level pipelines.

## Stage 0: Python Environment

Create or refresh the project environment first:

```bash
make venv
```

`make venv` requires `uv` and runs:

```bash
uv sync --extra dev --frozen
```

That keeps `.venv/` in sync with `pyproject.toml` and `uv.lock`, including
runtime dependencies such as Pillow and dev tools such as pytest and ruff.

Do not treat a pre-existing `.venv/` as proof that dependencies are installed;
run `make venv` after pulling changes to `pyproject.toml` or `uv.lock`.

## Stage 1: Open Setup

Fresh clone:

```bash
make venv
bin/doctor-open
bin/setup-open-plan
bin/setup-open
bin/doctor-open --strict
```

`bin/setup-open` covers the open-source path only. It stops before:

- local PsyQ staging
- disc extraction
- EMI unpack
- Ghidra planning against local extracted assets

Run the setup pieces directly when needed:

- `bin/setup-submodules`
- `bin/setup-private-assets` for the optional private cache workspace only
- `bin/setup-aspsx`
- `bin/setup-native-tools`
- `bin/setup-psx-toolchain`
- `bin/setup-match-tools`

The open setup pipeline currently runs these tasks:

1. `submodules`
2. `aspsx-binaries`
3. `native-tools`
4. `psx-toolchain`
5. `match-tools`

The `psx-toolchain` task downloads and stages:

- `toolchains/psn00b_toolchain/bin/mipsel-none-elf-gcc`
- `toolchains/psn00bsdk/`
- `toolchains/gcc-2.7.2-psx/gcc`

Before `bin/setup-open`, `bin/doctor-open --strict` is expected to report those
toolchains as missing. After `bin/setup-open`, strict open doctor should pass
unless a host tool or download/build step failed.

## Stage 2: Local / Proprietary Inputs

Default BOF3 work currently uses PsyQ 4.7. Download the public Arthus
`psyq-4.7-converted-full` archive into the private asset cache, extract it, and
stage the active SDK with:

```bash
bin/download-psyq
```

This writes source media and extracted source trees under
`external/private-assets/psyq/4.7/`, then stages the repo-consumable SDK under
`toolchains/psyq/4.7/`.

To stage an existing repo-local tree or archive instead:

```bash
bin/setup-psyq --archive inputs/psyq-4.7-converted-full.7z
```

Also supported:

- `bin/setup-psyq --source-root inputs/psyq-4.7-converted-full`
- `bin/setup-psyq --version 4.6 --archive inputs/psyq-4.6.zip`
- `bin/setup-psyq --version 4.6 --source-root inputs/psyq-4.6`
- `bin/setup --psyq-version 4.7 --psyq-archive ... --disc-archive ...` for the full setup path

Active runtime paths:

- disc input: `inputs/disc/`
- PsyQ SDK: `toolchains/psyq/<version>/`

`external/private-assets/` is a private download and cache workspace, not the
normal runtime location. The build reads the staged SDK from `toolchains/psyq/<version>/`.
Use `bin/configure -DBOF3_PSYQ_VERSION=4.6` for another staged SDK version, or
`bin/configure -DBOF3_PSYQ_ROOT=/absolute/path/to/psyq` for an explicit SDK root.

## Stage 3: Disk / EMI Lifecycle

Typical sequence:

```bash
bin/disk-extract
bin/emi-unpack
```

Related commands:

- `bin/emi-pack`: repack unpacked EMI folders back into the tree
- `bin/disk-rebuild`: rebuild a disc image from the extracted project
- `bin/disk-checksums`: generate checksums for staged disc images
- `bin/disk-verify`: verify staged disc images against those checksums

`bin/emi-unpack` and `bin/emi-pack` operate over the tree by default.
`bin/disk-checksums` is the companion command for `bin/disk-verify`.

## Stage 4: Inventory

Use the maintained pipeline when you want the full artifact family:

```bash
bin/inventory-build
```

Individual inventory commands remain available:

- `bin/inventory-scan`
- `bin/inventory-group`
- `bin/inventory-slot-map`
- `bin/inventory-emi-catalog`
- `bin/inventory-overlay-catalog`
- `bin/inventory-overlay-clusters`
- `bin/inventory-unique-overlay-map`
- `bin/inventory-entry-tables`
- `bin/inventory-project-plan`
- `bin/inventory-render-metadata`
- `bin/inventory-import-ghidra-symbols`

Raw Ghidra export reshaping still runs through
`bin/inventory-import-ghidra-symbols`. Dedicated export scripts are not implemented yet.

## Stage 5: Ghidra

Typical sequence:

```bash
bin/ghidra-plan
bin/ghidra-bootstrap
```

Other supported commands:

- `bin/ghidra-summary`
- `bin/ghidra-ui`
- `bin/ghidra-install-extensions`

Generated planning artifacts live under `out/ghidra-bootstrap/`.

## Stage 6: Match / Asset Work

Function matching:

- `bin/match-init`
- `bin/match-build`
- `bin/match-diff`
- `bin/match-report`

Asset extraction and review:

- `bin/emi-extract`
- `bin/emi-review`
- `bin/emi-extract-archive`
- `bin/emi-extract-tree`
- `bin/emi-render-title`
- `bin/emi-render-status`
- `bin/emi-preview`

Image workflows require Pillow in the active Python environment.

## Make Targets

The Makefile intentionally exposes only high-level lifecycle targets:

- `make venv`, `make doctor-open`, `make doctor`
- `make setup-open`, `make setup`
- `make extract`, `make inventory`, `make ghidra`
- `make configure`, `make build`, `make test`, `make fmt`

Use `bin/*` for individual tools.

## Current Caveats

- Full heavy verification can be slow; run the checks that match the workflow you changed before submitting.
