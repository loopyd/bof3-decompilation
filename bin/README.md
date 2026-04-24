# bin

This directory is the maintained primary human-facing command surface.

Use `make` only for setup, test, format, build, and high-level pipelines.
Most wrappers dispatch into the maintained Python implementation under
`tools/python/`.

Pipelines are command-backed recipes. Use `pipeline <name> --plan` to inspect
the exact commands before running a workflow.

## Setup

- `pipeline --list`
- `pipeline setup-open --plan`
- `pipeline extract-assets --plan`
- `pipeline inventory-refresh --plan`
- `pipeline ghidra-bootstrap --plan`
- `pipeline ghidra-ready --plan`
- `pipeline decomp-ready --plan`
- `setup-open`, `setup-open-plan`, `setup-plan`, `setup`
- `setup-submodules`, `setup-private-assets`
- `setup-aspsx`, `setup-native-tools`, `setup-psx-toolchain`, `setup-match-tools`
- `download-psyq`, `setup-psyq`
- `bin/doctor --profile open`, `bin/doctor --profile full`
- `bin/doctor --profile decomp`, `bin/doctor --profile ghidra`
- `bin/doctor-open` is an alias for `bin/doctor --profile open`

Doctor profiles validate phases of the same full reverse-engineering project:
open setup, Ghidra bootstrap, decomp/matching, or the full reverse project.
Ghidra and decomp dependencies are project dependencies, not optional extras.

## Disk / EMI

- `disk-extract`, `disk-rebuild`, `disk-verify`, `disk-checksums`
- `emi-unpack`, `emi-pack`
- `emi-extract`, `emi-review`
- `emi-extract-archive`, `emi-extract-tree`
- `emi-render-title`, `emi-render-status`, `emi-preview`

Notes:

- `emi-unpack` and `emi-pack` operate over the tree by default.
- `disk-checksums` pairs with `disk-verify`.
- Image workflows require Pillow.

## Inventory

- `inventory-build`
- `inventory-scan`, `inventory-group`
- `inventory-slot-map`, `inventory-emi-catalog`
- `inventory-overlay-catalog`, `inventory-overlay-clusters`
- `inventory-unique-overlay-map`, `inventory-entry-tables`
- `inventory-project-plan`, `inventory-render-metadata`
- `inventory-import-ghidra-symbols`

## Ghidra

- `ghidra-plan`, `ghidra-bootstrap`, `ghidra-import-project`, `ghidra-summary`
- `ghidra-export-symbols`
- `ghidra-ui --ghidra-home /path/to/ghidra`
- `ghidra-install-extensions --user-dir /path/to/.ghidra_XX.Y <extension>`

Heavy Ghidra workflows should use `GHIDRA_HOME` or commands that accept
`--ghidra-home` instead of embedding workstation-specific paths.
For the system install here, `GHIDRA_HOME=/opt/ghidra` is the expected value.
Sandboxed headless runs may also need writable `XDG_CONFIG_HOME` and
`XDG_CACHE_HOME` values, and the active Ghidra user dir must include the
`ghidra_psx_ldr` extension for `PSX:LE:32:default`.
Ghidra symbol export automation is available through `ghidra-export-symbols`.
`inventory-import-ghidra-symbols` reshapes those exports, and `decomp-ready`
runs export, import, and decomp-profile verification as one inspectable recipe.

## Match

- `asm-diff-one <bof3/src/.../func_XXXXXXXX.c>`
- `match-init`, `match-build`, `match-diff`, `match-report`

The high-level reverse path is clone, verify open dependencies, stage local
disc/PsyQ inputs, run `pipeline ghidra-ready`, export symbols from Ghidra, run
`pipeline decomp-ready`, then iterate through decomp and match loops.

## Build

- `configure`, `build`
- `maspsx-cc` is the canonical maspsx wrapper used by CMake

Composable equivalents:

- `pipeline build-ready`
- `pipeline match-loop`

## Internal

- `_python_entry` is an internal wrapper helper, not a user command

`external/private-assets/` is optional. It is a private download and cache
workspace, not a normal runtime dependency.

`build/` and `out/` are generated trees. Use the commands here to populate and
refresh them instead of treating them as maintained source directories.
