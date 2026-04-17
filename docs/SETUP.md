# Setup

## Human Entry Points

Use one of these:

- `make venv`
- `make doctor`
- `make doctor-open`
- `make setup-plan`
- `make setup-open-plan`
- `make setup`
- `make setup-open`
- `make setup-submodules`
- `make setup-aspsx`
- `make setup-native-tools`
- `make setup-psx-toolchain`
- `make setup-match-tools`
- `make inventory`
- `make fmt`
- `make format-python`
- `make configure`
- `make build`

Or use the direct command wrappers under `bin/`:

- `bin/setup`
- `bin/setup-open`
- `bin/setup-open-plan`
- `bin/setup-plan`
- `bin/setup-submodules`
- `bin/setup-aspsx`
- `bin/setup-native-tools`
- `bin/setup-psx-toolchain`
- `bin/setup-match-tools`
- `bin/setup-psyq`
- `bin/doctor`
- `bin/doctor-open`
- `bin/inventory-scan`
- `bin/inventory-group`
- `bin/ghidra-plan`
- `bin/ghidra-bootstrap`
- `bin/configure`
- `bin/build`

`bin/bof3` remains available as a legacy compatibility wrapper.

## Typical Flow

1. Create the Python environment.
2. Run `make doctor-open`.
3. Preview the fresh-clone bring-up with `make setup-open-plan`.
4. Run `make setup-open` or the smaller `make setup-submodules`, `make setup-aspsx`, `make setup-native-tools`, `make setup-psx-toolchain`, and `make setup-match-tools` targets.
5. Once local inputs are available, run `make setup-psyq`, extract the disc, unpack, then run `make inventory`.
6. Configure and build with CMake.

Use `make fmt` to format repo-owned `bof3/` C/H sources plus the Python tooling.
Use `make format-python` when you only want the Python formatter.

`make setup-open` stops before the local PsyQ stage, disc extraction, unpack, and Ghidra planning.
Use it to initialize submodules, public toolchains, and the repo-owned helper tools on a fresh clone before supplying proprietary inputs.

`make doctor` remains the full-workspace check once those local inputs and generated outputs exist.

## PsyQ

PsyQ is local and proprietary. The repo does not download it for you.

Use one of:

- `make setup PSYQ_SOURCE=/path/to/psyq-4.0`
- `make setup-psyq PSYQ_ARCHIVE=/path/to/psyq-4.7-converted-full.7z`
- `bin/setup-psyq --source-root /path/to/psyq-4.0`
- `bin/setup-psyq --archive /path/to/psyq-4.7-converted-full.7z`

The staging step normalizes text-file line endings for non-Windows workflows and creates lowercase compatibility aliases used by the build.

## ASPSX

The public ASPSX reference bundle is downloaded separately from the proprietary PsyQ SDK.

- `make setup-aspsx` downloads only the canonical `psyq4.0` bundle.
- `bin/setup-aspsx --all-versions` downloads the full public version matrix when you need it for research.

## Output Locations

- generated planning manifests: `out/ghidra-bootstrap/`
- unpacked EMI payloads: `out/emi_raw/`
- staged toolchains: `toolchains/`
- local supplied inputs: `inputs/`
