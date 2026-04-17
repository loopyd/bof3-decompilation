# rebof3-simple

`rebof3-simple` is a stripped-down BOF3 decomp workspace with two public surfaces:

- `make ...` for common workflows
- `bin/...` for direct POSIX entrypoints

The canonical top-level layout is:

- `bin/`: human-facing commands
- `bof3/`: recovered game code
- `tools/`: repo-owned tooling, with Python automation under `tools/python/`
- `third_party/`: vendored or external tool repos
- `inputs/`: local supplied inputs such as disc images and proprietary SDK archives
- `toolchains/`: downloaded or staged SDKs and compilers
- `out/`: generated manifests, extracted data, and reports
- `docs/`: human documentation and reverse-engineering specs
- `cmake/`: PSX build and toolchain files

## Quick Start

Bootstrap the Python environment:

```bash
make venv
```

Check the fresh-clone open setup path:

```bash
make doctor-open
# or
bin/doctor-open
```

Preview the fresh-clone setup flow:

```bash
make setup-open-plan
# or
bin/setup-open-plan
```

Set up only the open-source pieces first:

```bash
make setup-open
# or
bin/setup-open
```

This stops before local PsyQ staging, disc extraction, unpack, and Ghidra planning while still initializing submodules, staging public toolchains, and building the open-source helper tools.

If you want the same bring-up one step at a time, use:

```bash
make setup-submodules
make setup-aspsx
make setup-native-tools
make setup-psx-toolchain
make setup-match-tools
```

Once local proprietary inputs are available, stage PsyQ separately:

```bash
make setup-psyq PSYQ_SOURCE=/path/to/psyq-4.0
# or
bin/setup-psyq --source-root /path/to/psyq-4.0
```

Refresh the generated inventory and Ghidra import manifest:

```bash
make inventory
# or
bin/ghidra-bootstrap
```

Configure and build the PSX target:

```bash
make configure
make build
# or
bin/configure
bin/build
```

Format the Python tooling:

```bash
make format-python
```

Format all repo-owned source we maintain directly:

```bash
make fmt
```

## Notes

- `make setup-open` is the recommended fresh-clone bring-up path.
- `make setup` remains available when local PsyQ and disc inputs are ready.
- `bin/setup-open`, `bin/setup-open-plan`, `bin/setup-submodules`, `bin/setup-aspsx`, `bin/setup-native-tools`, `bin/setup-psx-toolchain`, `bin/setup-match-tools`, `bin/doctor-open`, and `bin/setup-psyq` are the preferred setup entrypoints.
- `bin/doctor`, `bin/inventory-scan`, `bin/inventory-group`, `bin/ghidra-plan`, `bin/ghidra-bootstrap`, `bin/configure`, and `bin/build` remain the maintained workflow entrypoints outside setup.
- `bin/bof3` remains available as a legacy compatibility wrapper.
- `bin/maspsx-cc` is the canonical maspsx wrapper used by CMake.
- `make fmt` formats repo-owned `bof3/` C/H sources with `clang-format` and `tools/python/` with `ruff format`.
- `make format-python` runs only the Python formatter.
- PsyQ is still a local proprietary input. Pass `PSYQ_SOURCE` / `PSYQ_ARCHIVE` or place a local copy under `inputs/`.
- `make setup-aspsx` stages only the canonical public ASPSX/PsyQ 4.0 bundle under `toolchains/aspsx-psyq-binaries/` and exposes it to maspsx through `third_party/maspsx/aspsx/psyq`.
- Use `bin/setup-aspsx --all-versions` only if you need the broader public version matrix for research or toolchain comparison.
- `scripts/` and `scripts/legacy/` are compatibility surfaces, not the preferred workflow.
