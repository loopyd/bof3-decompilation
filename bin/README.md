# bin

This directory is the human-facing command surface.

Preferred entrypoints:

- `bin/setup-open`
- `bin/setup-open-plan`
- `bin/setup-submodules`
- `bin/setup-private-assets` (optional private download/cache workspace only)
- `bin/setup-aspsx`
- `bin/setup-native-tools`
- `bin/setup-psx-toolchain`
- `bin/setup-match-tools`
- `bin/setup-psyq`
- `bin/doctor-open`
- `bin/doctor`
- `bin/inventory-scan`
- `bin/inventory-group`
- `bin/ghidra-plan`
- `bin/ghidra-bootstrap`
- `bin/ghidra-summary`
- `bin/configure`
- `bin/build`

Other maintained entrypoints:

- `bin/setup`
- `bin/setup-plan`

Legacy compatibility entrypoints:

- `bin/bof3`
- `bin/inventory`

`external/private-assets/` is not a normal runtime dependency.
Importer flows may use it as an optional private workspace before staging active inputs under `inputs/disc/` or `toolchains/psyq/4.7/`.
