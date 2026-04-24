# rebof3-simple

`rebof3-simple` is a stripped-down BOF3 decomp workspace.

This repo is in a migration stabilization pass. Treat `bin/` plus
`tools/python/` as the maintained command and implementation surfaces.

Primary UX:

- `bin/*` commands for detailed tools
- `make` targets for setup, test, format, build, and high-level pipelines

## Quick Start

Create or sync the Python environment:

```bash
make venv
```

This requires `uv` and runs `uv sync --extra dev --frozen`.

Check the open-source setup path:

```bash
bin/doctor --profile open
bin/pipeline setup-open --plan
bin/setup-open-plan
bin/setup-open
bin/doctor --profile open --strict
```

Once local proprietary inputs are available, continue with the full asset flow:

```bash
bin/download-psyq
export GHIDRA_HOME=/opt/ghidra
bin/pipeline ghidra-ready --plan
bin/pipeline ghidra-ready
bin/pipeline decomp-ready --plan
bin/pipeline decomp-ready
bin/configure
bin/build
```

Sandboxed headless Ghidra runs may also need writable `XDG_CONFIG_HOME` and
`XDG_CACHE_HOME` values, plus the `ghidra_psx_ldr` extension in the active
Ghidra user directory. See `docs/SETUP.md` and `docs/TROUBLESHOOTING.md` for
the exact local setup.

## Main Workflows

### Setup

- `bin/doctor --profile open`: validate the fresh-clone open setup phase
- `bin/doctor --profile full`: validate the complete reverse project state
- `bin/doctor --profile decomp`: validate the decomp/matching phase
- `bin/doctor --profile ghidra`: validate the Ghidra bootstrap phase
- `bin/doctor-open`: alias for `bin/doctor --profile open`
- `bin/pipeline --list`: list composable pipeline entry points
- `bin/pipeline setup-open --plan`: inspect the task-level setup-open pipeline
- `bin/pipeline ghidra-ready --plan`: inspect extraction, inventory, and Ghidra bootstrap
- `bin/pipeline decomp-ready --plan`: inspect Ghidra symbol export/import and decomp verification
- `bin/setup-open`: fresh-clone open setup
- `bin/setup-submodules`: init submodules only
- `bin/setup-aspsx`: stage public ASPSX reference binaries
- `bin/setup-native-tools`: build native helper tools
- `bin/setup-psx-toolchain`: stage the open PSX toolchain
- `bin/setup-match-tools`: build matching helpers
- `bin/download-psyq`: download/cache the default PsyQ 4.7 source archive under `external/private-assets/`, then stage it
- `bin/setup-psyq`: stage a repo-local PsyQ tree or archive into `toolchains/psyq/<version>/`
- `bin/setup`: full setup when local proprietary inputs are ready

### Disk / EMI

- `bin/disk-extract`: extract the staged BOF3 disc image
- `bin/emi-unpack`: unpack EMI archives from the extracted tree
- `bin/emi-pack`: repack unpacked EMI folders back into the tree
- `bin/disk-rebuild`: rebuild a disc image from the extracted project
- `bin/disk-checksums`: generate checksums for staged disc images
- `bin/disk-verify`: verify staged disc images against those checksums

`bin/emi-unpack` and `bin/emi-pack` operate over the tree by default.

### Inventory / Ghidra

- `bin/inventory-build`: refresh the maintained inventory artifact family
- `bin/inventory-scan`, `bin/inventory-group`
- `bin/inventory-slot-map`, `bin/inventory-emi-catalog`
- `bin/inventory-overlay-catalog`, `bin/inventory-overlay-clusters`
- `bin/inventory-unique-overlay-map`, `bin/inventory-entry-tables`
- `bin/inventory-project-plan`, `bin/inventory-render-metadata`
- `bin/inventory-import-ghidra-symbols`: reshape Ghidra symbol export artifacts
- `bin/ghidra-plan`, `bin/ghidra-bootstrap`, `bin/ghidra-import-project`, `bin/ghidra-summary`
- `bin/ghidra-export-symbols`
- `bin/ghidra-ui --ghidra-home /path/to/ghidra`
- `bin/ghidra-install-extensions --user-dir /path/to/.ghidra_XX.Y <extension>`

Composable equivalents:

- `bin/pipeline extract-assets`
- `bin/pipeline inventory-refresh`
- `bin/pipeline ghidra-ready`
- `bin/pipeline decomp-ready`

Ghidra symbol export automation is available through
`bin/ghidra-export-symbols`. `bin/inventory-import-ghidra-symbols` reshapes
those exports, and the `decomp-ready` pipeline runs export, import, and
decomp-profile verification as one inspectable recipe.

### Match

- `bin/asm-diff-one bof3/src/core/emi/func_80162178.c`: compile one source object and diff it against original asm
- `bin/match-init`
- `bin/match-build`
- `bin/match-diff`
- `bin/match-report`

See `docs/DECOMP_WORKFLOW.md` for the one-function decomp loop.

### Asset Review

- `bin/emi-extract`, `bin/emi-review`
- `bin/emi-extract-archive`, `bin/emi-extract-tree`
- `bin/emi-render-title`, `bin/emi-render-status`
- `bin/emi-preview`

Image workflows require Pillow in the active Python environment.

## Layout And Ownership

- `bin/`: canonical human-facing commands
- `tools/python/`: maintained repo-owned Python implementations behind `bin/`
- `docs/DECOMP_WORKFLOW.md`: focused compile/decompile/asm-diff loop
- `inputs/`: user-owned local runtime inputs such as disc images and local source archives
- `external/private-assets/`: optional private download and cache workspace only
- `toolchains/`: staged or downloaded SDKs and compilers, including PsyQ under `toolchains/psyq/<version>/`
- `build/`: generated local build tree
- `out/`: generated extraction, inventory, planning, and review artifacts

## Notes

- `make` intentionally stays small; use `bin/*` for detailed tools.
- The optional `external/private-assets/` path is a private download and cache workspace, not a normal runtime dependency.
- Treat `out/` as the generated-artifact tree for current workflows.
