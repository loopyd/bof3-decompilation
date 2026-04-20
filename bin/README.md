# bin

This directory is the maintained primary human-facing command surface.

Use `make *` only as convenience aliases for the common wrappers here.
Most wrappers dispatch into the maintained Python implementation under
`tools/python/`.

## Setup

- `setup-open`, `setup-open-plan`, `setup-plan`, `setup`
- `setup-submodules`, `setup-private-assets`
- `setup-aspsx`, `setup-native-tools`, `setup-psx-toolchain`, `setup-match-tools`
- `setup-psyq`
- `doctor-open`, `doctor`

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

- `ghidra-plan`, `ghidra-bootstrap`, `ghidra-summary`
- `ghidra-ui`, `ghidra-install-extensions`

Raw Ghidra export reshaping currently still flows through
`inventory-import-ghidra-symbols`.

## Match

- `match-init`, `match-build`, `match-diff`, `match-report`

## Build

- `configure`, `build`
- `maspsx-cc` is the canonical maspsx wrapper used by CMake

## Compatibility-Only

- `bof3`
- `inventory`
- `_python_entry` is an internal wrapper helper, not a user command

`external/private-assets/` is optional. It is a private download and cache
workspace, not a normal runtime dependency.

`build/` and `out/` are generated trees. Use the commands here to populate and
refresh them instead of treating them as maintained source directories.
