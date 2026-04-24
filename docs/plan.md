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
- Headless Ghidra symbol export automation is implemented through
  `bin/ghidra-export-symbols` and the `decomp-ready` pipeline.
- Build and match pipeline recipes are implemented as `build-ready` and
  `match-loop`.
- Docs have been updated for current setup and workflow guidance.
- Relevant tests are passing as of the latest verification run.

## In Progress

- No active implementation task is delegated at the moment.

## Next

- Add pipeline argument support for configurable runs.
- Consolidate wrappers so command-backed pipelines share one obvious path.
- Clean up setup architecture and audit dependencies.

## Later/Risks

- Keep migration scope isolated to `rebof3-simple`; avoid coupling new work to
  legacy `rebof3`.
- Validate Ghidra automation on clean machines, not only developer workstations.
- Keep the plan concise and human-editable; avoid turning it into a changelog.
