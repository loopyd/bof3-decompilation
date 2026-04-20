# Setup

Use `bin/*` as the primary interface. `make *` targets are convenience aliases.

## Stage 1: Open Setup

Fresh clone:

```bash
make venv
bin/doctor-open
bin/setup-open-plan
bin/setup-open
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

## Stage 2: Local / Proprietary Inputs

Stage PsyQ once local inputs are available:

```bash
bin/setup-psyq --archive inputs/psyq-4.7-converted-full.7z
```

Also supported:

- `bin/setup-psyq --source-root inputs/psyq-4.7-converted-full`
- `bin/setup --psyq-archive ... --disc-archive ...` for the full setup path

Active runtime paths:

- disc input: `inputs/disc/`
- PsyQ SDK: `toolchains/psyq/4.7/`

`external/private-assets/` is optional. It is a private download and cache workspace,
not the normal runtime location.

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

## Convenience Aliases

The main `make` aliases are:

- `make doctor-open`, `make setup-open-plan`, `make setup-open`
- `make setup-psyq`, `make setup`
- `make disk-extract`, `make emi-unpack`, `make emi-pack`
- `make inventory-build`, `make ghidra-bootstrap`
- `make match-init`, `make match-build`, `make match-diff`, `make match-report`
- `make configure`, `make build`

## Current Caveats

- `bin/bof3` and the aggregate CLI surface are compatibility-only.
- `bin/inventory` is also compatibility-oriented; prefer the explicit inventory and Ghidra commands.
- Full heavy verification was intentionally skipped for now.
