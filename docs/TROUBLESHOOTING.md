# Troubleshooting

## `make doctor` fails

Read the failing lines directly. The doctor command reports missing host tools, missing local inputs, missing staged toolchains, and missing generated outputs.

Common fixes:

- install `cmake`
- install `cargo`
- place a BOF3 disc set under `inputs/disc/`
- stage PsyQ with `make setup-psyq ...`
- stage open toolchains with `make setup-open`

## PsyQ is missing

The repo expects a local PsyQ 4.0 tree or archive. Pass one of:

- `PSYQ_SOURCE=/path/to/psyq-4.0`
- `PSYQ_ARCHIVE=/path/to/psyq-4.7-converted-full.7z`

## Build cannot find `maspsx-cc`

Use the canonical wrapper at `bin/maspsx-cc`.

## Old docs still mention removed root aliases

Prefer:

- `out/` for generated output
- `toolchains/` for staged SDKs and compilers
- `inputs/` for local disc and proprietary inputs
- `tools/python/` for repo-owned Python tooling
- `third_party/` for vendored tool repos
