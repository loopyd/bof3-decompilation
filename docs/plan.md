# rebof3-simple Migration Plan

This living plan tracks the migration work for `rebof3-simple` only. It is not
for legacy `rebof3`.

## Completed

- Core primitives are in place for command execution, profile loading, and
  pipeline orchestration.
- Doctor profiles are available for environment and dependency checks.
- Generic pipeline CLI exists as the main command surface for running pipelines.
- `setup-open` pipeline is implemented.
- Reverse command-backed pipelines are implemented.
- Headless Ghidra project import automation is implemented through
  `bin/ghidra-import-project` and the `ghidra-ready` pipeline.
- Ghidra import now stages hardlinked/copied inputs under
  `out/ghidra-import-staging/` so manifest program names survive Ghidra 12
  headless import.
- Headless Ghidra symbol export automation is implemented through
  `bin/ghidra-export-symbols` and the `decomp-ready` pipeline.
- Build, match, and harness pipeline recipes are implemented as `build-ready`,
  `match-loop`, and `harness-ready`.
- Docs have been updated for current setup and workflow guidance.
- Latest verification on 2026-05-17 passed: full pytest suite, Ruff,
  compileall, diff whitespace, and `bin/ghidra-import-project --help`.

## In Progress

- `BATTLE.EMI#3` function lifting (at `bof3/src/modules/battle/03/`).

## Next

- Add pipeline argument support for configurable runs.
- Consolidate wrappers so command-backed pipelines share one obvious path.
- Clean up setup architecture and audit dependencies.
- Establish a documentation review cadence (every ~2 weeks).

## Later/Risks

- Keep migration scope isolated to `rebof3-simple`; avoid coupling new work to
  legacy `rebof3`.
- Optimize full Ghidra bootstrap runtime. Current import is correct but still
  expensive because each manifest entry needs distinct loader options.
- Validate Ghidra automation on clean machines, not only developer workstations.
  Latest local validation confirmed `/opt/ghidra`, writable headless
  config/cache requirements, and active `ghidra_psx_ldr` extension
  requirements.
- Keep the plan concise and human-editable; avoid turning it into a changelog.
