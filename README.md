# rebof3-simple

`rebof3-simple` is a stripped-down BOF3 decomp workspace.

This repo is in a migration stabilization pass. Treat `bin/` plus
`tools/python/` as the maintained command and implementation surfaces.
`scripts/legacy/` remains compatibility-only.

Primary UX:

- `bin/*` commands
- `make *` convenience aliases for common wrappers

Compatibility-only surfaces still exist, but they are not the preferred workflow.

## Quick Start

Create the Python environment:

```bash
make venv
```

Check the open-source setup path:

```bash
bin/doctor-open
bin/setup-open-plan
bin/setup-open
```

Once local proprietary inputs are available, continue with the full asset flow:

```bash
bin/setup-psyq --archive inputs/psyq-4.7-converted-full.7z
bin/disk-extract
bin/emi-unpack
bin/ghidra-bootstrap
bin/configure
bin/build
```

## Main Workflows

### Setup

- `bin/setup-open`: fresh-clone open setup
- `bin/setup-submodules`: init submodules only
- `bin/setup-aspsx`: stage public ASPSX reference binaries
- `bin/setup-native-tools`: build native helper tools
- `bin/setup-psx-toolchain`: stage the open PSX toolchain
- `bin/setup-match-tools`: build matching helpers
- `bin/setup-psyq`: stage local PsyQ into `toolchains/psyq/4.7/`
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
- `bin/inventory-import-ghidra-symbols`: reshape raw Ghidra export artifacts
- `bin/ghidra-plan`, `bin/ghidra-bootstrap`, `bin/ghidra-summary`
- `bin/ghidra-ui`, `bin/ghidra-install-extensions`

Raw Ghidra export reshaping currently still flows through
`bin/inventory-import-ghidra-symbols`. Dedicated export scripts are not
implemented yet.

### Match

- `bin/match-init`
- `bin/match-build`
- `bin/match-diff`
- `bin/match-report`

### Asset Review

- `bin/emi-extract`, `bin/emi-review`
- `bin/emi-extract-archive`, `bin/emi-extract-tree`
- `bin/emi-render-title`, `bin/emi-render-status`
- `bin/emi-preview`

Image workflows require Pillow in the active Python environment.

## Layout And Ownership

- `bin/`: canonical human-facing commands
- `tools/python/`: maintained repo-owned Python implementations behind `bin/`
- `scripts/legacy/`: compatibility-only legacy automation surface
- `inputs/`: user-owned local runtime inputs such as disc images and local source archives
- `external/private-assets/`: optional private download and cache workspace only
- `toolchains/`: staged or downloaded SDKs and compilers, including local PsyQ under `toolchains/psyq/4.7/`
- `build/`: generated local build tree
- `out/`: generated extraction, inventory, planning, and review artifacts

## Notes

- `make` targets mirror the main `bin/*` commands as convenience aliases.
- `bin/bof3` and the aggregate compatibility CLI surface are compatibility-only.
- `bin/inventory` remains a compatibility wrapper; prefer the explicit inventory and Ghidra commands above.
- The optional `external/private-assets/` path is a private download and cache workspace, not a normal runtime dependency.
- `tmp/` and `processed/` may still appear in older docs or tooling; treat `out/` as the current generated-artifact tree.
- Full heavy verification was intentionally skipped for now. Do not treat the current docs as claiming a full validation matrix.
