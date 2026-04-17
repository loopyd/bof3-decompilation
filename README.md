# rebof3-simple

`rebof3-simple` is a stripped-down BOF3 decomp workspace with one public surface:

- `make ...` for common workflows
- `bin/...` for direct commands

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

Check host tools and workspace state:

```bash
make doctor
```

Preview the setup pipeline:

```bash
make setup-plan
```

Set up the full workspace:

```bash
make setup PSYQ_SOURCE=/path/to/psyq-4.0
```

Or set up the open-source pieces first:

```bash
make setup-open
```

Refresh the generated inventory and Ghidra import manifest:

```bash
make inventory
```

Configure and build the PSX target:

```bash
make configure
make build
```

Format the Python tooling:

```bash
make format-python
```

Format all repo-owned source we maintain directly:

```bash
make fmt
```

Run one setup task directly when needed:

```bash
bin/bof3 setup task psyq --psyq-source-root /path/to/psyq-4.0
```

## Notes

- `make setup` is the happy path.
- `bin/bof3` is the canonical CLI entrypoint.
- `bin/doctor`, `bin/setup`, `bin/inventory`, `bin/ghidra-bootstrap`, and `bin/setup-psyq` are thin convenience wrappers.
- `bin/maspsx-cc` is the canonical maspsx wrapper used by CMake.
- `make fmt` formats repo-owned `bof3/` C/H sources with `clang-format` and `tools/python/` with `ruff format`.
- `make format-python` runs only the Python formatter.
- PsyQ is still a local proprietary input. Pass `PSYQ_SOURCE` / `PSYQ_ARCHIVE` or place a local copy under `inputs/`.
- `make setup-aspsx` stages only the canonical public ASPSX/PsyQ 4.0 bundle under `toolchains/aspsx-psyq-binaries/` and exposes it to maspsx through `third_party/maspsx/aspsx/psyq`.
- Use `bin/bof3 toolchain aspsx download --all-versions` only if you need the broader public version matrix for research or toolchain comparison.
- `scripts/` and `scripts/legacy/` are compatibility surfaces, not the preferred workflow.
