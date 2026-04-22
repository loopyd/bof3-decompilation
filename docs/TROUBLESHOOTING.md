# Troubleshooting

## `make doctor` fails

Read the failing lines directly. The doctor command reports missing host tools, missing local inputs, missing staged toolchains, and missing generated outputs.

On a fresh clone before PsyQ, disc images, extraction, and unpack are available, run `make doctor-open` instead of `make doctor`.

Common fixes:

- install `uv`
- install `cmake`
- install `ninja`
- install `cargo`
- place a BOF3 disc set under `inputs/disc/`
- stage PsyQ with `bin/download-psyq` or `bin/setup-psyq ...`
- stage open toolchains with `make setup-open` or `bin/setup-open`

`external/private-assets/` is optional. If it is absent, the normal public setup still works.

## `bin/doctor-open --strict` reports missing open toolchains

This is expected before the open setup pipeline has run. Stage the public
toolchains with:

```bash
make venv
bin/setup-open-plan
bin/setup-open
bin/doctor-open --strict
```

The missing paths should be populated by `bin/setup-open`:

- `toolchains/psn00b_toolchain/bin/mipsel-none-elf-gcc`
- `toolchains/gcc-2.7.2-psx/gcc`

If they are still missing afterwards, rerun only that task with:

```bash
bin/setup-psx-toolchain --force
```

## Python cannot import `PIL`

The project declares Pillow in `pyproject.toml`, but an old or manually created
`.venv/` can be out of sync. Refresh it with:

```bash
make venv
```

This requires `uv` and runs `uv sync --extra dev --frozen`.

## PsyQ is missing

The active SDK path is `toolchains/psyq/<version>/`; the default version is 4.7.
For the public Arthus 4.7 converted-full archive, run:

```bash
bin/download-psyq
```

For an existing repo-local tree or archive, use:

- `bin/setup-psyq --source-root inputs/psyq-4.7-converted-full`
- `bin/setup-psyq --archive inputs/psyq-4.7-converted-full.7z`
- `bin/setup-psyq --version 4.6 --archive inputs/psyq-4.6.zip`

`bin/download-psyq` caches source media and extracted source trees under
`external/private-assets/psyq/<version>/` before staging the active SDK.

## Build cannot find `maspsx-cc`

Use the canonical wrapper at `bin/maspsx-cc`.

## Old docs still mention removed root aliases

Prefer:

- `out/` for generated output
- `toolchains/` for staged SDKs and compilers
- `inputs/` for local disc and proprietary inputs
- `tools/python/` for repo-owned Python tooling
- `third_party/` for vendored tool repos
