# Setup

Use `bin/*` as the primary tool interface. `make` is intentionally small and
only covers setup, test, format, build, and high-level pipelines.

Pipelines are command-backed recipes. Inspect any maintained pipeline before
running it with:

```bash
bin/pipeline <name> --plan
```

Heavy Ghidra workflows should use commands that accept `GHIDRA_HOME` or an
explicit `--ghidra-home` path instead of embedding workstation-specific paths.

## Doctor Profiles

Use `bin/doctor --profile <name>` to validate a workflow phase:

- `bin/doctor --profile open`: fresh clone, open-source submodules, host tools,
  native tools, matching helpers, and open PSX toolchain
- `bin/doctor --profile full`: complete reverse project state, including local
  disc/PsyQ inputs, Ghidra bootstrap outputs, Ghidra symbol imports, and decomp
  tools
- `bin/doctor --profile decomp`: decompilation loop state; currently equivalent
  to `full`
- `bin/doctor --profile ghidra`: Ghidra bootstrap state before symbol import and
  decomp loops
- `bin/doctor-open`: alias for `bin/doctor --profile open`

Add `--strict` when a check should fail on any reported issue. Ghidra and
decomp dependencies are part of the full reverse-engineering project; profiles
only validate different phases of the workflow.

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
bin/doctor --profile open
bin/pipeline --list
bin/pipeline setup-open --plan
bin/setup-open-plan
bin/setup-open
bin/doctor --profile open --strict
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

Before `bin/setup-open`, `bin/doctor --profile open --strict` is expected to
report those toolchains as missing. After `bin/setup-open`, strict open doctor
should pass unless a host tool or download/build step failed.

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
The PSX build uses `toolchains/gcc-2.7.2-psx/gcc` for C and the staged
PSn00b binutils for assembly, archive, link, and EXE conversion.

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

Ghidra symbol export automation runs through `bin/ghidra-export-symbols`.
`bin/inventory-import-ghidra-symbols` reshapes those exported artifacts into
repo inventory indexes.

## Stage 5: Ghidra

Typical sequence:

```bash
bin/pipeline ghidra-ready --plan
bin/pipeline ghidra-ready
bin/doctor --profile ghidra --strict
```

Other supported commands:

- `bin/ghidra-summary`
- `bin/ghidra-import-project --ghidra-home /path/to/ghidra`
- `bin/ghidra-export-symbols --ghidra-home /path/to/ghidra`
- `bin/ghidra-ui --ghidra-home /path/to/ghidra`
- `bin/ghidra-install-extensions --user-dir /path/to/.ghidra_XX.Y <extension>`

Generated planning artifacts live under `out/ghidra-bootstrap/`.
`bin/ghidra-import-project` also accepts `GHIDRA_HOME` when `--ghidra-home` is
not passed.

For the system Ghidra install on this workstation, use:

```bash
export GHIDRA_HOME=/opt/ghidra
```

Headless Ghidra also needs a writable settings/cache area. In sandboxed runs
where `$HOME/.config` or `/var/tmp` are not writable, use explicit temporary
locations:

```bash
export XDG_CONFIG_HOME=/tmp/rebof3-ghidra-config
export XDG_CACHE_HOME=/tmp/rebof3-ghidra-cache
```

The BOF3 import manifest uses the PSX language (`PSX:LE:32:default`), so the
active Ghidra user dir must have the `ghidra_psx_ldr` extension installed. If
the normal user config already has it and the sandbox uses a temporary
`XDG_CONFIG_HOME`, copy it into the temporary Ghidra user dir:

```bash
bin/ghidra-install-extensions \
  --user-dir /tmp/rebof3-ghidra-config/$USER-ghidra/ghidra_12.0.4_DEV \
  ~/.config/ghidra/ghidra_12.0.4_DEV/Extensions/ghidra_psx_ldr
```

`bin/ghidra-import-project` stages hardlinked or copied files under
`out/ghidra-import-staging/` before import. This preserves the manifest's
human-readable program names with Ghidra 12, which does not support a
headless `-programName` import flag.

## Stage 6: Ghidra Symbols

Use Ghidra to review the bootstrapped project and export symbols, then reshape
the raw export for repo inventory:

```bash
bin/pipeline decomp-ready --plan
bin/pipeline decomp-ready
```

`bin/pipeline decomp-ready` exports symbols from the headless Ghidra project,
imports them into repo inventory indexes, and verifies the decomp profile.

## Stage 7: Match / Asset Work

Function matching:

- `bin/asm-diff-one bof3/src/core/emi/func_80162178.c`
- `bin/harness verify function bof3/src/core/emi/func_80162178.c`
- `bin/match-init`
- `bin/match-build`
- `bin/match-diff`
- `bin/match-report`

For the maintained one-function decomp loop, see `docs/DECOMP_WORKFLOW.md`.

Build, match, harness, and later whole-binary parity recipes are available as
`bin/pipeline build-ready`, `bin/pipeline match-loop`,
`bin/pipeline harness-ready`, and `bin/pipeline binary-parity`. Inspect them
first with `bin/pipeline <name> --plan`.

Asset extraction and review:

- `bin/emi-extract`
- `bin/emi-review`
- `bin/emi-extract-archive`
- `bin/emi-extract-tree`
- `bin/emi-render-title`
- `bin/emi-render-status`
- `bin/emi-preview`

Image workflows require Pillow in the active Python environment.

## Full Reverse Workflow

The full reverse/decompilation path is:

1. Clone `rebof3-simple` and run `make venv`.
2. Verify the open phase with `bin/doctor --profile open`, then run
   `bin/setup-open`.
3. Manually provide local disc inputs under `inputs/disc/` and stage PsyQ with
   `bin/download-psyq` or `bin/setup-psyq`.
4. Inspect and run `bin/pipeline ghidra-ready --plan`, then
   `bin/pipeline ghidra-ready`.
5. Export symbols from Ghidra.
6. Inspect and run `bin/pipeline decomp-ready --plan`, then
   `bin/pipeline decomp-ready`.
7. Iterate with `bin/asm-diff-one`, `bin/match-build`, `bin/match-diff`, and
   `bin/match-report`.

## Make Targets

The Makefile intentionally exposes only high-level lifecycle targets:

- `make venv`, `make doctor-open`, `make doctor`
- `make setup-open`, `make setup`, `make pipeline`
- `make extract`, `make inventory`, `make ghidra`
- `make configure`, `make build`, `make test`, `make fmt`

Use `bin/*` for individual tools.

## Current Caveats

- Full heavy verification can be slow; run the checks that match the workflow you changed before submitting.
