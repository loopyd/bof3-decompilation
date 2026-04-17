# Setup

## Human Entry Points

Use one of these:

- `make venv`
- `make doctor`
- `make setup-plan`
- `make setup`
- `make inventory`
- `make fmt`
- `make format-python`
- `make configure`
- `make build`

Or use the direct command wrappers under `bin/`:

- `bin/bof3`
- `bin/doctor`
- `bin/setup`
- `bin/inventory`
- `bin/ghidra-bootstrap`
- `bin/setup-psyq`

## Typical Flow

1. Create the Python environment.
2. Run `make doctor`.
3. Stage toolchains and SDKs with `make setup` or `make setup-open`.
4. Run `make inventory` after extraction and unpack.
5. Configure and build with CMake.

Use `make fmt` to format repo-owned `bof3/` C/H sources plus the Python tooling.
Use `make format-python` when you only want the Python formatter.

## PsyQ

PsyQ is local and proprietary. The repo does not download it for you.

Use one of:

- `make setup PSYQ_SOURCE=/path/to/psyq-4.0`
- `make setup-psyq PSYQ_ARCHIVE=/path/to/psyq-4.7-converted-full.7z`
- `bin/setup-psyq --source-root /path/to/psyq-4.0`

The staging step normalizes text-file line endings for non-Windows workflows and creates lowercase compatibility aliases used by the build.

## ASPSX

The public ASPSX reference bundle is downloaded separately from the proprietary PsyQ SDK.

- `make setup-aspsx` downloads only the canonical `psyq4.0` bundle.
- `bin/bof3 toolchain aspsx download --all-versions` downloads the full public version matrix when you need it for research.

## Output Locations

- generated planning manifests: `out/ghidra-bootstrap/`
- unpacked EMI payloads: `out/emi_raw/`
- staged toolchains: `toolchains/`
- local supplied inputs: `inputs/`
