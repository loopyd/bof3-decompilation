# Repo Layout

Use these names as canonical during the migration stabilization pass:

- `bin/`
  - maintained human-facing command surface
- `bof3/`
  - recovered game code
- `tools/`
  - repo-owned maintained tooling under `tools/python/`
- `third_party/`
  - vendored tool repos and helper projects
- `inputs/`
  - user-owned local runtime inputs, including the active disc set under `inputs/disc/`
- `toolchains/`
  - downloaded or staged SDKs and compilers, including `toolchains/psyq/<version>/`
- `external/private-assets/`
  - optional private download, processing, and cache workspace only
- `build/`
  - generated local build tree
- `out/`
  - generated manifests, extracted payloads, reports, and scratch output
  - treat this as the current generated-artifact root during migration
- `docs/`
  - setup docs and reverse-engineering knowledge

Notes:

- Prefer `bin/*` commands for all maintained workflows.
- Use `out/` for generated artifacts in current workflows.
