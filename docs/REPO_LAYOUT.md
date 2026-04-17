# Repo Layout

Use these names as canonical:

- `bin/`
  - the command surface humans should run
- `bof3/`
  - recovered game code
- `tools/`
  - repo-owned tooling under `tools/python/`
- `third_party/`
  - vendored tool repos and helper projects
- `inputs/`
  - local runtime inputs, including the active disc set under `inputs/disc/`
- `toolchains/`
  - downloaded or staged SDKs and compilers, including `toolchains/psyq-original/4.0/`
- `external/private-assets/`
  - optional private download, processing, and cache workspace only
- `out/`
  - generated manifests, extracted payloads, reports, and scratch output
- `docs/`
  - setup docs and reverse-engineering knowledge
